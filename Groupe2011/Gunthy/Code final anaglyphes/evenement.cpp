#include "evenement.h"

LRESULT CALLBACK evenement(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(handle != NULL)
    {
        int idFenetre = GetWindowLong(handle, GWL_USERDATA);

        if(idFenetre == 1)
        {
            return evenement_principale(handle, message, wParam, lParam);
        }
        else
        {
            return evenement_menu(handle, message, wParam, lParam);
        }
    }

    return DefWindowProc(handle, message, wParam, lParam);
}

LRESULT evenement_principale(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DESTROY:
            end_event();
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
        {
            redimensionner(handle, LOWORD(lParam),HIWORD(lParam));
            update_gl();
            return 0;
        }

        case WM_LBUTTONDOWN:
            changer_activation_camera();
        return 0;

        case WM_MOUSEMOVE:
        {
            static POINT old = {0,0};
            POINT current;
            current.x = GET_X_LPARAM(lParam);
            current.y = GET_Y_LPARAM(lParam);

            action_mouse_move(current.x - old.x, old.y - current.y);

            old = current;

            return 0;
        }

        case WM_MOUSEWHEEL:
            action_zoom(LOWORD(lParam)>0);
        return 0;

        default:
            return DefWindowProc(handle, message, wParam, lParam);
    }
}

LRESULT evenement_menu(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_HSCROLL:
        case WM_COMMAND:
            evenement_bouton(handle,message,wParam,lParam);
            return 0;

        default:
            return DefWindowProc(handle, message, wParam, lParam);
    }
}

int recuperer_evenement()
{
    return GetMessage(&(g_fenetre->message), NULL, 0, 0);
}

void traiter_evenement()
{
    // On traite les touches du clavier pressees
    evenement_clavier();

    // On s'occupe des autres messages
    TranslateMessage(&(g_fenetre->message));
    DispatchMessage(&(g_fenetre->message));
}

void evenement_bouton(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    // On fait un switch du bouton
    UINT id=LOWORD(wParam);

    switch(id)
    {
        // La camera
        case ID_FREEFLY : action_change_camera(0); break ;
        case ID_TRACKBALL : action_change_camera(1); break;
        case ID_ANAGLYPHE : action_change_anaglyphe(SendMessage(g_fenetre->anaglyphe, BM_GETCHECK, 0, 0) == BST_CHECKED); break;

        case ID_RETRO : action_activer_retro(SendMessage(g_fenetre->retro, BM_GETCHECK, 0, 0) == BST_CHECKED); break;
        case ID_T_RETRO :
            if(HIWORD(wParam) == EN_CHANGE)
                action_change_edit(g_fenetre->t_retro, g_fenetre->s_retro, 1, 0.1, 50., &(event_status->dist_retro));
            if(HIWORD(wParam) == EN_UPDATE)
                action_update_edit(g_fenetre->t_retro);
             break;

        case ID_T_DISTANCE :
            if(HIWORD(wParam) == EN_CHANGE)
                action_change_edit(g_fenetre->t_distance, g_fenetre->s_distance, 1, 0.1, 50., &(event_status->distance));
            if(HIWORD(wParam) == EN_UPDATE)
                action_update_edit(g_fenetre->t_distance);
             break;
        case ID_NB_X :
            if(HIWORD(wParam) == EN_CHANGE)
                action_change_nb_x();
            break;
        case ID_NB_Y :
            if(HIWORD(wParam) == EN_CHANGE)
                action_change_nb_y();
            break;
        case ID_NB_Z :
            if(HIWORD(wParam) == EN_CHANGE)
                action_change_nb_z();
            break;

        case ID_ORDI : action_default_distance(0.5); break;
        case ID_SALLE : action_default_distance(5.); break;
        case ID_AMPHI : action_default_distance(20.); break;

        // La maille
        case ID_PARCOURIR : action_parcourir(); break;
        case ID_GENERER : action_generer(); break;

        case ID_ATOME :
            if(HIWORD(wParam) == CBN_SELCHANGE)
                 action_change_atome();
            break;
        case ID_COULEUR :
            if(HIWORD(wParam) == CBN_SELCHANGE)
                action_change_couleur();
            break;

        case ID_DEFAULT : action_defaut(handle); break;
        case ID_HELP : action_aide(handle); break;

        default :
            break;
    }

    if((HWND)lParam == g_fenetre->s_retro)
        action_change_edit(g_fenetre->t_retro, g_fenetre->s_retro, 0, 0.1, 50., &(event_status->dist_retro));
    if((HWND)lParam == g_fenetre->s_distance)
        action_change_edit(g_fenetre->t_distance, g_fenetre->s_distance, 0, 0.1, 50., &(event_status->distance));
    if((HWND)lParam == g_fenetre->s_taille)
        action_change_taille();
    if((HWND)lParam == g_fenetre->s_espace_atome)
        action_change_espace();
}

void evenement_clavier()
{
    static clock_t precedent = clock();

    // Est-ce important de s'occuper du clavier ?
    POINT pos_souris;
    RECT rect_gl;
    GetCursorPos(&pos_souris);
    GetWindowRect(g_fenetre->gl, &rect_gl);
    if(is_in(pos_souris, rect_gl))
    {
        if(event_status->camera_active)
        {
            // On regarde l'etat de differentes touches.
            // 1 - la touche est de type maintien, on le met dans le event_status
            action_controle(GetKeyState(VK_CONTROL) < 0);


            // 2 - la touche est de type joystick, on realise son action
            // Test de key repeat : temps d attente ?
            clock_t courant = clock();
            // Si oui :
            if(courant - precedent > NB_CLOCKS_ECART_TOUCHE)
            {
                precedent = courant;
                if(GetKeyState(VK_UP) < 0 || GetKeyState('z') < 0 || GetKeyState('Z') < 0)
                {
                    action_up();
                }
                if(GetKeyState(VK_DOWN) < 0 || GetKeyState('s') < 0 || GetKeyState('S') < 0)
                {
                    action_down();
                }
                if(GetKeyState(VK_LEFT) < 0 || GetKeyState('q') < 0 || GetKeyState('Q') < 0)
                {
                    action_left();
                }
                if(GetKeyState(VK_RIGHT) < 0 || GetKeyState('d') < 0 || GetKeyState('D') < 0)
                {
                    action_right();
                }
                if(GetKeyState(VK_NEXT))
                {
                    action_zoom(1);
                }
                if(GetKeyState(VK_PRIOR))
                {
                    action_zoom(0);
                }
            }
        }
    }
    else    // on desactive la camera
    {
        event_status->camera_active = 0;
    }
}

int is_in(POINT p, RECT r)
{
    return (p.x >= r.left && p.x <= r.right && p.y >= r.top && p.y <= r.bottom);
}
